FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

RUN apt-get update && apt-get install -y sudo zsh git curl wget locales clangd

# install omz theme
RUN rm -rf /root/.oh-my-zsh && RUNZSH=no sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

COPY .zshrc /root/.zshrc

WORKDIR /workspace
