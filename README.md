# Introduction

My notes for OSCP. Read at your own peril :computer:

OSCP has 25 modules, following the blog post [here](https://blog.leonardotamiano.xyz/tech/oscp-technical-guide/), they seem to nicely sort into the following broad concepts, seperated into two categories:

1. Technological Concepts:
    - Web
    - Linux
    - Windows
    - Active Directory
2. Techniques:
    - Password Attacks
    - Using Existing Exploits
    - Port Forwarding and Pivoting
    - Active Directory
    - Client-Side Attacks
    - Report Writing
    - Misc.

OSCP is not meant to go into extreme depth in any one area, rather provide an introduction to a breadth of areas. This is disappointing by perhaps makes it more tractable. Anyone can generalize :smile:.

In particular, it requires **basic enumeration** and **basic exploitation**. You'll never need to write a custom exploit, only make small changes to existing exploits to fit the scenario.

Next, let's summarize the content covered in each module

# Web

**What's not covered?**
Basic web network protocol like TCP and HTTP aren't covered. No problem, I think I've got them under control.

**What is covered?**
Basic web vulnerabilities. One of which recieves a whole module to itself: SQL injections.

## SQL Injections
After finding some way we can send SQL commands, we might try to escape the normally executed `SELECT foo FROM bar` by completing the first query and subsequently injecting our own malicious query. Apparently, this can also give RCE!

A classic example might be something like:
```
' OR 1=1 --
1 UNION SELECT first_name, password FROM users #
```

## Directory Traversals
Allows an attacker to explore the filesystem of a web server by attempting to move up the file system with many `../`. This was all those weird URLs that the honeypot encountered. Maybe I should go back and write that up...

An example might look like:
```
http://target.com/vuln?page=../../../../../../../etc/passwd
```

## Local File Inclusion
Allows an attacker to insert some local file that shouldn't be accessible into an accessible page on the webserver. Again, we might aim for /etc/passwd for example. This is why we don't run our server as root!

## Remote File Inclusion
Same as above, but now... a remote file :sparkle:. Maybe this is a python script giving a reverse shell or something.

## File Upload Vulnerabilities
An attacker could upload a malicious file to achieve RCE or leak some data

## OS Command Injection
Attacker injects some command line code that is then executed by the OS. 

## Cross-Site Scripting (XSS)
Attacker injects some code that is then executed by the web browsers of all clients who visit the page. Most commonly this is used to deface websites as far as I'm aware.

## Tools
These is mention of tools such as:
- `gobuster` for enumerating virtual hosts, directories, files and domain names.
- `sqlmap` for automatically exploiting SQL injectable fields.
- `BurpSuite` for good old HTTP request crafting.

## Game Plan
During web exploitation there's two broad steps.

1. Gaining a foothold: The goal here is to get a reverse shell. There's many ways to get here, maybe exploting HTTP, TCP, web vulnerabilities, etc., but the goal is the same: shell!
2. Privilege escalation: The goal here is to upgrade our shell from a crappy user like apache to my good friend Mr. `root`. Maybe we won't always get all the way there, but more privileges is better.

# Linux

*God, I hope there's not too many surprises here...*

When learning about the web, we're regailed with narratives of FTP, SSH, SMB and RDP. They're all protocols we might exploit for gaining our initial foothold. Eventually, we want to escalate though, and this requires an understanding of the OS, rather than the services it's hosting.

This is where Linus Torvald and his compadres come in. For exploiting Linux, we want to be comfortable:
- Moving around the filesystem with `cd` and `ls`.
- Managing permissions with `chmod`, `chown` and `chgrp`.
- Finding files with... uhm... `find`.
- Understanding environment variables.
- Understanding the role of PATH.
- Understanding how to work with privileges `sudo`.
- Understanding SUID/GUID binaries.
- Understanding how passwords are stored in `passwd` and shadow files (wtf is that?)
- Understanding cronjobs
- Understanding basic tools like `ssh`, `ftp`, `curl`, `nc`, `python` (yay) and `gcc` (yay).
- Understanding how to view processes and open ports with `ps` and `netstat`.

These look ok to me. 
