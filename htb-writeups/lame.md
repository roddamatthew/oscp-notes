---
marp: true
theme: uncover
---
# Lame
11/06/2025

---

- Easy Linux box
- Tags:
    - Network enumeration
    - Metasploit

---

`nmap`

- Discovered 4 services:
    1. 22/tcp : openssh 4.3
    2. 21/tcp : vsFTPD 2.3.4
    3. 139/tcp : smb 3.0.*
    4. 445/tcp : smb 3.0.20

---

Potential Vulns:
- vsFTPD has a famous back door apparently!
    - If you use a smiley face as your username it starts a shell on port 6500!
    - Isn't that fun!
- There's an exploit for this in metasploit

---

`msfconsole`

- First time using metasploit. I feel like a real script kiddie now!
- Can search for services to find exploits w/:

`search <service_name> [<version>]`

- Can then see info about the exploits with: `info`
- Or use an exploit with: `use`
- Usually you'll then have to set some params with:

`set <param> <value>`

---

# vsFTPD exploit

- Use the exploit and set the correct target with `set rhost <target_ip>`
- It fails! Oh no!
- Later we'll find out why

---

# Down the list we go

- smb 3.0.20 has a RCE exploit
- Again this just exists in metasploit
    - Nice!
- Use the exploit
- Don't forget to set rhost
- Don't forget to set lhost to VPN accessible IP
    - I messe that up at first
- We're in

---

# Priv Escalation

`> whoami`
`> root`

nice.

`find / user.txt`
`find / root.txt`

Done!

---

# Post exploitation

- With `netstat -tnlp` we see there's a bunch of ports open!
    - Why aren't they all visible from nmap scan?
    - Probably because of a firewall
    - Firewall probably also blocked that shell on port 6500.
    - Well done net defense people!