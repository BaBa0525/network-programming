# Homework 1 - MIRCD

A simple IRC server to run on WeeChat

## Run Locally

Clone the project

```bash
  git clone https://github.com/BaBa0525/network-programming
```

Go to the project directory

```bash
  cd network-programming/Hw1
```

Compile

```bash
  make
```

Start the server

```bash
  ./server <port>
```

## Usage/Examples

You can use [WeeChat](https://weechat.org/), a console-based IRC client, to connect to the server

- Add a server: `/server add <servername> <hostname>/<port>`

  - For example, `/server add mircd localhost/10004`

- Set user nickname for a server: `/set irc.server.<servername>.nicks "<nickname>"`

  - For example, `/set irc.server.mircd.nicks "user1"`

- Connect to a server: `/connect <servername>`

  - For example, `/connect mircd`

- List users on the server: `/users`

- List channels on the server: `/list`

- Join a channel: `/join <#channel>`

- Leave a channel: `/part`

- Close a buffer and leave a channel (or a server): `/close`

- Set channel topic: `/topic <topic>`. You can only do this when you are in a channel.

- Terminate weechat: `/quit`

- Send messages in a channel: Simply type the message you want to send, and all the users in the channel should receive the message.
