# GhostClient
You have the server? So nice! Let's download the client.

**Hey, read this**: This is a client for the GhostServer. You need to have the server ready in order to use this client.
If you haven't, just go to [GhostServer Repo](https://github.com/JulioFresneda/GhostServer) and follow the instructions.

## Installation
You have the installer in /Installer/. Just install it and follow the initialization steps.

### Build
The installer just copies the files from /Build/ in your program files. If you want the raw files, pick them from the /Build/ folder.

### Source
The source code is everything that is not in /Installer/ or /Build/.

## Initialization
Okay, here is the thing: Just now I am too lazy to add a initial user/password login GUI in the app. So, you will have to write some things manually. No big deal.

First, open with admin privileges the "conf.ini" file in the installation path. You have to fill this values:
- password: The password that you chose when adding an user to the server
- userID: The user that you chose when adding an user to the server
- domain: The domain you already configured in DuckDNS.

That's it! Ez pz for you, and for me, that I don't have to code that shit in the app.

I'm in fucking love with this app because of the effort dedicated, so I hope you enjoy it :) See you!