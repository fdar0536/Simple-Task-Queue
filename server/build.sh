#!/bin/sh

rm -rf *.protos
mkdir -p protos
cd ../protos

for f in *.proto
do
    protoc --go_out=. --go_opt=paths=source_relative --go-grpc_out=. --go-grpc_opt=paths=source_relative $f
done

mv *.proto ../server/proto
cd ../server
go build
