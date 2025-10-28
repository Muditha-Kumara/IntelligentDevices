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

variable "iot_thing_name" {
  description = "Name of the IoT Thing"
  default     = "vehicle-1"
}

variable "alert_email" {
  description = "Email address to receive alerts"
}

variable "alert_sms" {
  description = "Phone number to receive SMS alerts (E.164 format, e.g. +1234567890)"
}

resource "aws_iot_thing" "device" {
  name = var.iot_thing_name
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

resource "aws_sns_topic" "alerts" {
  name = "vehicle_alerts"
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
  handler       = "main"
  runtime       = "go1.x"
  filename      = "lambda.zip"
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

output "iot_thing_name" {
  value = aws_iot_thing.device.name
}

output "dynamodb_table_name" {
  value = aws_dynamodb_table.events.name
}

output "sns_topic_arn" {
  value = aws_sns_topic.alerts.arn
}
