# Build stage
FROM public.ecr.aws/lambda/provided:al2023-x86_64 AS builder

RUN dnf update -y && \
    dnf install -y wget tar git ca-certificates

ENV GO_VERSION=1.21.5
RUN wget https://go.dev/dl/go${GO_VERSION}.linux-amd64.tar.gz && \
    tar -C /usr/local -xzf go${GO_VERSION}.linux-amd64.tar.gz && \
    rm go${GO_VERSION}.linux-amd64.tar.gz

ENV PATH="/usr/local/go/bin:${PATH}"

WORKDIR /app
COPY go.mod go.sum ./
RUN go mod download

COPY . .

# Build completely static binary
RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build \
    -a -installsuffix cgo \
    -ldflags '-w -s -extldflags "-static"' \
    -tags "netgo osusergo static_build" \
    -trimpath \
    -o main .

# Test with scratch - this will fail if not static
FROM scratch AS test
COPY --from=builder /app/main /main
COPY --from=builder /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/

# Final stage
FROM public.ecr.aws/lambda/provided:al2023-x86_64
COPY --from=builder /app/main ${LAMBDA_TASK_ROOT}/
COPY bootstrap /var/runtime/bootstrap
RUN chmod +x /var/runtime/bootstrap
CMD ["main"]
