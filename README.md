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
