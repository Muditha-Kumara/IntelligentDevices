#!/bin/bash
# Build and push Lambda container image to ECR in eu-north-1

set -e

AWS_REGION="eu-north-1"
ACCOUNT_ID="201940484677"
REPO_NAME="vehicle-event-processor"
IMAGE_TAG="latest"
LAMBDA_DIR="$(dirname "$0")/lambda"

# Build Docker image
cd "$LAMBDA_DIR"
docker build -t ${REPO_NAME}:${IMAGE_TAG} .
cd -

# Authenticate Docker to ECR
aws ecr get-login-password --region ${AWS_REGION} | \
  docker login --username AWS --password-stdin ${ACCOUNT_ID}.dkr.ecr.${AWS_REGION}.amazonaws.com

# Create ECR repository (ignore error if exists)
aws ecr create-repository --repository-name ${REPO_NAME} --region ${AWS_REGION} || true

# Tag image for ECR
docker tag ${REPO_NAME}:${IMAGE_TAG} ${ACCOUNT_ID}.dkr.ecr.${AWS_REGION}.amazonaws.com/${REPO_NAME}:${IMAGE_TAG}

# Push image to ECR
docker push ${ACCOUNT_ID}.dkr.ecr.${AWS_REGION}.amazonaws.com/${REPO_NAME}:${IMAGE_TAG}

echo "Image pushed to ECR: ${ACCOUNT_ID}.dkr.ecr.${AWS_REGION}.amazonaws.com/${REPO_NAME}:${IMAGE_TAG}"