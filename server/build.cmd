rmdir /S /Q protos
mkdir protos
cd ../protos

rem %1 = path to protoc
rem %2 = path to go plugin for protoc
set PATH=%1;%2;%PATH%
for %%f in (*.proto) do (
    protoc --go_out=. --go_opt=paths=source_relative --go-grpc_out=. --go-grpc_opt=paths=source_relative %%f
)

move *.go ../server/protos
cd ../server
go build
