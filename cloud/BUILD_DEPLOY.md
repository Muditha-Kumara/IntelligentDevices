# Build and Deploy Instructions for Cloud Lambda

## Build Go Lambda Function
1. Change directory:
   ```bash
   cd cloud/lambda
   ```
2. Build the Go binary for Linux (required by AWS Lambda):
   ```bash
   GOOS=linux GOARCH=amd64 go build -o main main.go
   ```
3. Zip the binary:
   ```bash
   zip ../lambda.zip main
   ```

## Deploy Infrastructure with Terraform
1. Change to the cloud directory:
   ```bash
   cd ../
   ```
2. Initialize Terraform:
   ```bash
   terraform init
   ```
3. Apply Terraform (creates all AWS resources):
   ```bash
   terraform apply
   ```
   - Review and confirm the plan.

## Notes
- Update IAM policies for least privilege if needed.
- Set up SNS subscriptions (email/SMS) in AWS Console or extend Terraform.
- Make sure your AWS credentials are configured (`aws configure`).
- You can extend the Go Lambda and Terraform for more features as required.
