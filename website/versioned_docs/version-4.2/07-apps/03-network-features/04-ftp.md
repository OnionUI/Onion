---
slug: /network/ftp
---


# FTP

![](https://github.com/OnionUI/Onion/assets/47260768/7bfac01d-bfaa-4565-b10b-b2b2a0ea7f9c)

Much like the HTTP server, FTP provides a method of transferring files between your Miyoo Mini+ and other devices such as a PC/Phone/Tablet. You'll need an FTP client installed on the second device.


## Features

- File transfer

- Debugging and troubleshooting


## Enabling FTP access

You can either use the master toggle to turn the FTP server on with your Right/Left arrows, or press A to enter the submenu.

You'll find it in **Apps** › **Tweaks** › **Network** › **FTP: File server** and you'll get 2 options:

1. **Enable** - Activate FTP access

2. **Enable authentication** - Activate authentication (username/password)

![](https://github.com/OnionUI/Onion/assets/47260768/38aa375d-c2a1-40d5-9037-36a998858d9b)


## Logging in

:::note Default credentials
**Username:** `onion`  
**Password:** `onion`  
*We're using a new auth system, user defined passwords will come in a future update.*
:::

Once you've activated your FTP server in Tweaks you'll now be able to connect using software such as FileZilla, CuteFTP, WinSCP, FireFTP. You'll also be able to use an SFTP client to transfer files securely if SSH is also enabled.

You'll need the IP of the device, found below (**Tweaks** › **Network**)

![](https://github.com/OnionUI/Onion/assets/47260768/23ee6dbf-48c8-4484-b98b-9a8642b8fd49)

Exit tweaks to apply & start the FTP server


## Disabling authentication

Although this is not recommended on other peoples Wi-Fi, when you're at home on your own Wi-Fi this is absolutely fine.

Head over to **Tweaks** › **Network** › **FTP**:

![](https://github.com/OnionUI/Onion/assets/47260768/0cf75a65-654a-4b9a-815c-7c37d6d3f649)

:::info File corruption
On some FTP clients you may need to alter settings or binary files will become damaged when they're transferred, FileZilla for example may need to change the setting shown below (**Transfer** › **Transfer type** › **Binary**)

![](https://github.com/OnionUI/Onion/assets/47260768/62d13812-883c-466d-954a-5ce00b8306f4)
:::

If you have authentication turned on, anonymous connections will fail.


## Security

:::caution Network security
Although we've taken every precaution to offer as much security as possible, remember to keep your Onion safe. It is not recommended you use FTP on an insecure Wi-Fi network that is open or public as traffic is unencrypted, for this reason we strongly recommend you toggle FTP off when you're on the move!
:::
