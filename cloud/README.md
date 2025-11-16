# Cloud Backend for IntelligentDevices Vehicle Monitor


This README documents the cloud backend for the IntelligentDevices Vehicle Monitor project. For full project context and device-side setup, see [../README.md](../README.md).


## Overview
- IoT devices send telemetry and event data via HTTP POST to AWS API Gateway.
- API Gateway triggers AWS Lambda (Go, Docker image) for event processing.
- Lambda stores events in DynamoDB and sends notifications via SNS (email/SMS) for alerts/critical events.
- All infrastructure is provisioned and managed using Terraform.

## AWS Services Used
- **AWS API Gateway**: HTTP endpoint for device data ingestion.
- **AWS Lambda (Go, Docker image)**: Event processing and notification logic.
- **Amazon DynamoDB**: Event and telemetry data storage.
- **Amazon SNS**: Email and SMS notifications.
- **IAM**: Secure roles and permissions for services.
- **CloudWatch**: Logging and monitoring.
- **Amazon ECR**: Container registry for Lambda Docker images.

## Directory Structure
```
cloud/
  ├── main.tf            # Terraform root module
  ├── modules/           # Terraform modules for reusable resources
  ├── lambda/            # Go source code for Lambda functions with Docker
  └── README.md          # This file
```

## Setup Instructions
1. **Write Go Lambda Function**
   - Place Go code in `cloud/lambda/`.
   - Function should process IoT events, store in DynamoDB, and publish to SNS.
   - Add a `Dockerfile` to build the Lambda as a container image.

2. **Build and Push Docker Image**
   - Build the Docker image for your Go Lambda function:
     ```bash
     docker build -t <your-ecr-repo>:latest ./lambda
     ```
   - Authenticate Docker to ECR and push the image:
     ```bash
     aws ecr get-login-password --region <region> | docker login --username AWS --password-stdin <your-ecr-url>
     docker push <your-ecr-repo>:latest
     ```
   - Note the image URI for use in Terraform.

3. **Terraform Infrastructure**
   - Define AWS API Gateway resources for HTTP POST endpoint.
   - Deploy Lambda (Go, Docker image) with required IAM roles and ECR image URI.
   - Create DynamoDB table for event storage.
   - Set up SNS topics and subscriptions (email/SMS).
   - Enable CloudWatch logging.

4. **Deployment**
   - Install Terraform and AWS CLI.
   - Configure AWS credentials.
   - Run `terraform init` and `terraform apply` in the `cloud/` directory.

## Best Practices
- Use least privilege IAM policies.
- Secure API Gateway with authentication (API keys, IAM, or custom authorizer).
- Use environment variables for Lambda configuration.
- Modularize Terraform code for reusability.
- Enable logging and monitoring for all services.
- Use multi-stage Docker builds for small Lambda images.


## References
- [Project Root README](../README.md)
- [AWS API Gateway Documentation](https://docs.aws.amazon.com/apigateway/latest/developerguide/welcome.html)
- [AWS Lambda Go](https://docs.aws.amazon.com/lambda/latest/dg/go-programming-model.html)
- [Deploy Lambda with Docker](https://docs.aws.amazon.com/lambda/latest/dg/images-create.html)
- [Terraform AWS Provider](https://registry.terraform.io/providers/hashicorp/aws/latest/docs)

---
For questions or improvements, please update this README.
