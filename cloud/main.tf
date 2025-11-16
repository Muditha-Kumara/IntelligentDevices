terraform {
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = ">= 4.0"
    }
  }
}

provider "aws" {
  region = var.aws_region
}

variable "aws_region" {
  description = "AWS region to deploy resources"
  default     = "us-east-1"
}

variable "alert_email" {
  description = "Email address to receive alerts"
}

variable "alert_sms" {
  description = "Phone number to receive SMS alerts (E.164 format, e.g. +1234567890)"
}

resource "aws_dynamodb_table" "events" {
  name         = "vehicle_events"
  billing_mode = "PAY_PER_REQUEST"
  hash_key     = "device_id"
  attribute {
    name = "device_id"
    type = "S"
  }
}

resource "aws_cloudwatch_log_group" "sns_alerts" {
  name              = "/aws/sns/vehicle_alerts"
  retention_in_days = 14
}

resource "aws_sns_topic" "alerts" {
  name = "vehicle_alerts"
  delivery_policy = jsonencode({
    http = {
      defaultHealthyRetryPolicy = {
        minDelayTarget = 20
        maxDelayTarget = 20
        numRetries = 3
        numMaxDelayRetries = 0
        numNoDelayRetries = 0
        numMinDelayRetries = 0
        backoffFunction = "linear"
      }
      disableSubscriptionOverrides = false
    }
  })
}

resource "aws_sns_topic_subscription" "email" {
  topic_arn = aws_sns_topic.alerts.arn
  protocol  = "email"
  endpoint  = var.alert_email
}

resource "aws_sns_topic_subscription" "sms" {
  topic_arn = aws_sns_topic.alerts.arn
  protocol  = "sms"
  endpoint  = var.alert_sms
}

resource "aws_lambda_function" "event_processor" {
  function_name = "vehicle_event_processor"
  role          = aws_iam_role.lambda_exec.arn
  package_type  = "Image"
  image_uri     = var.lambda_image_uri
  environment {
    variables = {
      DYNAMODB_TABLE = aws_dynamodb_table.events.name
      SNS_TOPIC_ARN  = aws_sns_topic.alerts.arn
    }
  }
}

resource "aws_iam_role" "lambda_exec" {
  name = "lambda_exec_role"
  assume_role_policy = jsonencode({
    Version = "2012-10-17"
    Statement = [{
      Action = "sts:AssumeRole"
      Effect = "Allow"
      Principal = {
        Service = "lambda.amazonaws.com"
      }
    }]
  })
}

resource "aws_iam_role_policy_attachment" "lambda_dynamodb" {
  role       = aws_iam_role.lambda_exec.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonDynamoDBFullAccess"
}

resource "aws_iam_role_policy_attachment" "lambda_sns" {
  role       = aws_iam_role.lambda_exec.name
  policy_arn = "arn:aws:iam::aws:policy/AmazonSNSFullAccess"
}

resource "aws_iam_role_policy_attachment" "lambda_basic" {
  role       = aws_iam_role.lambda_exec.name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole"
}

variable "lambda_image_uri" {
  description = "URI of the Lambda container image in ECR"
  default     = "201940484677.dkr.ecr.us-east-1.amazonaws.com/vehicle-event-processor"
}

resource "aws_apigatewayv2_api" "vehicle_api" {
  name          = "vehicle-events-api"
  protocol_type = "HTTP"
}

resource "aws_apigatewayv2_integration" "lambda_integration" {
  api_id           = aws_apigatewayv2_api.vehicle_api.id
  integration_type = "AWS_PROXY"
  integration_uri  = aws_lambda_function.event_processor.invoke_arn
  integration_method = "POST"
  payload_format_version = "2.0"
}

resource "aws_apigatewayv2_route" "events_route" {
  api_id    = aws_apigatewayv2_api.vehicle_api.id
  route_key = "POST /vehicle/events"
  target    = "integrations/${aws_apigatewayv2_integration.lambda_integration.id}"
}

resource "aws_lambda_permission" "apigw_lambda" {
  statement_id  = "AllowAPIGatewayInvoke"
  action        = "lambda:InvokeFunction"
  function_name = aws_lambda_function.event_processor.function_name
  principal     = "apigateway.amazonaws.com"
  source_arn    = "${aws_apigatewayv2_api.vehicle_api.execution_arn}/*/*"
}

resource "aws_apigatewayv2_stage" "default" {
  api_id      = aws_apigatewayv2_api.vehicle_api.id
  name        = "$default"
  auto_deploy = true
}

output "api_endpoint" {
  value = aws_apigatewayv2_api.vehicle_api.api_endpoint
}

output "dynamodb_table_name" {
  value = aws_dynamodb_table.events.name
}

output "sns_topic_arn" {
  value = aws_sns_topic.alerts.arn
}
