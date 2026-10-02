FROM	alpine:latest

RUN		apk add --no-cache clang build-base zsh curl git

RUN		sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

COPY	. /app
WORKDIR	/app

RUN		make re