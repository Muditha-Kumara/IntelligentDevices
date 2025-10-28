package main

import (
	"context"
	"encoding/json"
	"fmt"
	"os"

	"github.com/aws/aws-lambda-go/lambda"
	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/dynamodb"
	"github.com/aws/aws-sdk-go/service/sns"
	"github.com/aws/aws-sdk-go/service/dynamodb/dynamodbattribute"
)

// Event structure matches your IoT device payload
// Adjust fields as needed
 type VehicleEvent struct {
	DeviceID   string  `json:"device_id"`
	EventType  string  `json:"event_type"`
	Level      string  `json:"level"`
	Timestamp  string  `json:"timestamp"`
	Data       map[string]interface{} `json:"data"`
}

func HandleRequest(ctx context.Context, event VehicleEvent) error {
	sess := session.Must(session.NewSession())
	db := dynamodb.New(sess)
	snsClient := sns.New(sess)

	// Store event in DynamoDB
	item, err := dynamodbattribute.MarshalMap(event)
	if err != nil {
		return fmt.Errorf("failed to marshal event: %v", err)
	}
	_, err = db.PutItem(&dynamodb.PutItemInput{
		TableName: aws.String(os.Getenv("DYNAMODB_TABLE")),
		Item:      item,
	})
	if err != nil {
		return fmt.Errorf("failed to put item: %v", err)
	}

	// Send notification if level is alert/critical
	if event.Level == "alert" || event.Level == "critical" {
		msg, _ := json.Marshal(event)
		_, err := snsClient.Publish(&sns.PublishInput{
			TopicArn: aws.String(os.Getenv("SNS_TOPIC_ARN")),
			Message: aws.String(string(msg)),
		})
		if err != nil {
			return fmt.Errorf("failed to publish SNS: %v", err)
		}
	}
	return nil
}

func main() {
	lambda.Start(HandleRequest)
}
