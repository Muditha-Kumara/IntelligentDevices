package main

import (
	"context"
	"encoding/json"
	"os"
	"sort"
	"time"

	"github.com/aws/aws-lambda-go/events"
	"github.com/aws/aws-lambda-go/lambda"
	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/dynamodb"
	"github.com/aws/aws-sdk-go/service/sns"
	"github.com/aws/aws-sdk-go/service/dynamodb/dynamodbattribute"
)


type VehicleEvent struct {
	DeviceID   string                 `json:"device_id" dynamodbav:"device_id"`
	EventType  string                 `json:"event_type" dynamodbav:"event_type"`
	Level      string                 `json:"level" dynamodbav:"level"`
	Timestamp  string                 `json:"timestamp" dynamodbav:"timestamp"`
	Data       map[string]interface{} `json:"data" dynamodbav:"data"`
}

// checkFrequencyLimit checks if the most recent event is within 1 minute of the new request
func checkFrequencyLimit(db *dynamodb.DynamoDB, deviceID string, currentTime time.Time) (bool, error) {
	// Query events for this device (table uses device_id as partition key only).
	input := &dynamodb.QueryInput{
		TableName:              aws.String(os.Getenv("DYNAMODB_TABLE")),
		KeyConditionExpression: aws.String("device_id = :deviceId"),
		ExpressionAttributeValues: map[string]*dynamodb.AttributeValue{
			":deviceId": { S: aws.String(deviceID) },
		},
	}

	var times []time.Time
	// Paginate through query results to collect timestamps
	err := db.QueryPages(input, func(page *dynamodb.QueryOutput, lastPage bool) bool {
		for _, item := range page.Items {
			tsAttr := item["timestamp"]
			if tsAttr == nil || tsAttr.S == nil {
				continue
			}
			t, err := time.Parse(time.RFC3339, *tsAttr.S)
			if err != nil {
				continue
			}
			times = append(times, t)
		}
		return !lastPage
	})
	if err != nil {
		return false, err
	}

	// If no events exist yet, allow the request
	if len(times) == 0 {
		return false, nil
	}

	// Sort timestamps descending (newest first)
	sort.Slice(times, func(i, j int) bool { return times[i].After(times[j]) })

	// Get the most recent event timestamp (index 0)
	mostRecentTime := times[0]

	// If the incoming event's timestamp is within 1 minute of the most recent saved event,
	// consider it happening too frequently and stop.
	timeDiff := currentTime.Sub(mostRecentTime)
	if timeDiff < 0 {
		timeDiff = -timeDiff // Handle case where new event is older than last event
	}
	return timeDiff <= 1*time.Minute, nil
}

func HandleRequest(ctx context.Context, req events.APIGatewayV2HTTPRequest) (events.APIGatewayV2HTTPResponse, error) {
	var event VehicleEvent
	if err := json.Unmarshal([]byte(req.Body), &event); err != nil {
		return events.APIGatewayV2HTTPResponse{
			StatusCode: 400,
			Body:       "Invalid request body",
		}, nil
	}

	sess := session.Must(session.NewSession())
	db := dynamodb.New(sess)
	snsClient := sns.New(sess)

	// Parse current timestamp
	currentTime, err := time.Parse(time.RFC3339, event.Timestamp)
	if err != nil {
		// If parsing fails, use current time
		currentTime = time.Now().UTC()
		event.Timestamp = currentTime.Format(time.RFC3339)
	}

	// Check frequency limit before processing
	shouldStop, err := checkFrequencyLimit(db, event.DeviceID, currentTime)
	if err != nil {
		return events.APIGatewayV2HTTPResponse{
			StatusCode: 500,
			Body:       "Failed to check frequency limit",
		}, nil
	}

	// If frequency limit exceeded, return "stop"
	if shouldStop {
		return events.APIGatewayV2HTTPResponse{
			StatusCode: 200,
			Body:       "stop",
		}, nil
	}

	// Store event in DynamoDB
	item, err := dynamodbattribute.MarshalMap(event)
	if err != nil {
		return events.APIGatewayV2HTTPResponse{
			StatusCode: 500,
			Body:       "Failed to marshal event",
		}, nil
	}
	_, err = db.PutItem(&dynamodb.PutItemInput{
		TableName: aws.String(os.Getenv("DYNAMODB_TABLE")),
		Item:      item,
	})
	if err != nil {
		return events.APIGatewayV2HTTPResponse{
			StatusCode: 500,
			Body:       "Failed to put item",
		}, nil
	}

	// Send notification if level is alert/critical
	if event.Level == "alert" || event.Level == "critical" {
		msg, _ := json.Marshal(event)
		_, err := snsClient.Publish(&sns.PublishInput{
			TopicArn: aws.String(os.Getenv("SNS_TOPIC_ARN")),
			Message: aws.String(string(msg)),
		})
		if err != nil {
			return events.APIGatewayV2HTTPResponse{
				StatusCode: 500,
				Body:       "Failed to publish SNS",
			}, nil
		}
	}

	return events.APIGatewayV2HTTPResponse{
		StatusCode: 200,
		Body:       "Event processed successfully",
	}, nil
}

func main() {
	lambda.Start(HandleRequest)
}
