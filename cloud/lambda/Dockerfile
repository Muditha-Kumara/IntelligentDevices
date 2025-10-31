FROM public.ecr.aws/lambda/go:1.x AS build
WORKDIR /src
COPY . .
RUN go build -o main main.go

FROM public.ecr.aws/lambda/go:1.x
COPY --from=build /src/main ${LAMBDA_TASK_ROOT}/main
CMD ["main"]