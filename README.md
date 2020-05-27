# yangl
### **Y**et **A**nother **N**ordVPN **G**UI for **L**inux

![yangl10](https://user-images.githubusercontent.com/2843765/83081840-54641e80-a08a-11ea-9b7d-814818a5e518.gif)

This is unofficial GUI for [NordVPN](https://nordvpn.com/) desktop client.
It's written for my own purpose, and I'm not affiliated to the [NordVPN](https://nordvpn.com/) team in no way. Used titles, logo and so on are the property of legal rights holders.

# Features
Being too lazy to use the provided CLI or even to bind it to keyboard shortcuts, I wanted a quick way to get the connection status or setup/break the connection with a few mouse clicks. These, basically, are the main features placed in the context menu of yangl's system tray icon:

![traymenu](https://user-images.githubusercontent.com/2843765/83050387-303a1a80-a055-11ea-9dc2-2bda8afccc13.png)

yangl's context menu provides two type of actions — the "builtin" and "custom". Both of them are just wrappers on an application and its command line arguments to be called with.

## Builtin actions
It's a list of predefined wrappers (and its combinations) for existent [NordVPN](https://nordvpn.com/) switches, llike Connect, Disconnect, Set notifications On/Off or Rate connection.

![builtin_actions](https://user-images.githubusercontent.com/2843765/83073321-ec590c80-a078-11ea-97c6-680487135ca5.png)

## Custom actions
An ability to run your own application/script. 
In my case, the [NordVPN](https://nordvpn.com/) client some time stops working after hibernation and it's the place to restart it with a single click:

![custom_actions](https://user-images.githubusercontent.com/2843765/83073053-73f24b80-a078-11ea-80c8-12de17a1ef2f.png)

## Action configuration
It's possible to configure any action in runtime:

![actions_settings](https://user-images.githubusercontent.com/2843765/83074195-735ab480-a07a-11ea-9a4c-89703bd9965d.gif)

* ***Title*** — action's visible title; 
* ***Application*** — (Custom actions only) — path to executable to be run;
* ***Arguments*** — list of parameters to pass to the application;
* ***Timeout*** — interval to wait for target application to be launched and/or to get any output from it;
* ***Menu*** — where to put the action in the tray icon menu. Each action can be placed in its related submenu ([NordVPN](https://nordvpn.com/) for builtin and Extra for custom actions), in the root context menu itself or hidden at all;
* ***Always show result*** — The result of performing of most of the actions is... Well, just performed action. There is no any text output I'm interested in or there is a dedicated GUI to display it. But some of actions are just requests for text info — like Show used settings or Account details. This flag is to define, should the text output displayed after each run or not. If not set, the result would be shown only in case of error (non zero exit code or application crash).

![txt_output](https://user-images.githubusercontent.com/2843765/83075996-6db29e00-a07d-11ea-967a-d4cf3ca9f4ae.png)

## Geo chart

![geo_chart](https://user-images.githubusercontent.com/2843765/83076757-c9c9f200-a07e-11ea-9483-c6cab633d920.png)

Simple map UI that allows to select location of the target [NordVPN](https://nordvpn.com/) server. Contains a list of available groups, countries and cities — no servers. I did not find the way to get it through the CLI and I'm too lazy to grab it from the [NordVPN web site](https://nordvpn.com/).

## Hints
While most of builtin actions are pretty straightforward, here are some notes/hints.
### Login/Logout
First of all, I don't want to bother with anything related to sensetive information, so there are no tools to manage the account. Please, handle your login/password by yourself within the good old CLI :)

### Pausing
Sometime it's necessary to temporarry switch the VPN off to access some (rarely visited) local resources or to run torrent, or whatever. Here are the Pause actions for this purpouse — use one of the predefined intervals or type your own one.

### Balloons flood

![spam](https://user-images.githubusercontent.com/2843765/83077628-79539400-a080-11ea-831c-bff9ffe9ca50.png)

To avoid being spammed by balloon messages on reconnection you may want to switch off NordVPN's notification. Or may not. It's up to you. But it's impossible to switch off yangl's norifications, at least for now.

### Tray Icon Tooltip
Shows advanced info about current connection — used server, uptime and so on:

![tooltip](https://user-images.githubusercontent.com/2843765/83079687-0ac50500-a085-11ea-82f5-bef3b91043b2.png)

# Build
See [BUILD.md] (TBD)

# Install 
See [INSTALL.md] (TBD)
