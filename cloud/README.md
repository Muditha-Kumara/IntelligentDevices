# Cloud Configuration for Vehicle Monitoring System

This README describes the cloud-side architecture and setup for integrating IoT devices with AWS using Go and Terraform.

## Overview
- IoT devices send telemetry and event data via MQTT.
- AWS IoT Core ingests device data and triggers event processing.
- AWS Lambda (Go) processes events and stores them in DynamoDB.
- AWS SNS sends email and SMS notifications to clients based on event severity.
- All infrastructure is provisioned using Terraform.

## AWS Services Used
- **AWS IoT Core**: Device connectivity and MQTT ingestion.
- **AWS Lambda (Go runtime)**: Event processing and notification logic.
- **Amazon DynamoDB**: Event and telemetry data storage.
- **Amazon SNS**: Email and SMS notifications.
- **IAM**: Secure roles and permissions for services.
- **CloudWatch**: Logging and monitoring.

## Directory Structure
```
cloud/
  ├── main.tf            # Terraform root module
  ├── modules/           # (Optional) Terraform modules for reusable resources
  ├── lambda/            # Go source code for Lambda functions
  └── README.md          # This file
```

## Setup Instructions
1. **Write Go Lambda Function**
   - Place Go code in `cloud/lambda/`.
   - Function should process IoT events, store in DynamoDB, and publish to SNS.

2. **Terraform Infrastructure**
   - Define AWS IoT Core resources (thing, policy, rule).
   - Deploy Lambda (Go) with required IAM roles.
   - Create DynamoDB table for event storage.
   - Set up SNS topics and subscriptions (email/SMS).
   - Enable CloudWatch logging.

3. **Deployment**
   - Install Terraform and AWS CLI.
   - Configure AWS credentials.
   - Run `terraform init` and `terraform apply` in the `cloud/` directory.

## Best Practices
- Use least privilege IAM policies.
- Secure device communication with certificates.
- Use environment variables for Lambda configuration.
- Modularize Terraform code for reusability.
- Enable logging and monitoring for all services.

## References
- [AWS IoT Core Documentation](https://docs.aws.amazon.com/iot/latest/developerguide/)
- [AWS Lambda Go](https://docs.aws.amazon.com/lambda/latest/dg/go-programming-model.html)
- [Terraform AWS Provider](https://registry.terraform.io/providers/hashicorp/aws/latest/docs)

---
For questions or improvements, please update this README.
