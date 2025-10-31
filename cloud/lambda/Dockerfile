FROM golang:1.21 AS builder
WORKDIR /app
COPY go.mod .
COPY go.sum .
COPY main.go .
RUN go build -o bootstrap main.go

FROM public.ecr.aws/lambda/go:latest
COPY --from=builder /app/bootstrap .
CMD ["bootstrap"]