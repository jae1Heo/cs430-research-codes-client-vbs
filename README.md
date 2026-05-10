# cs430-research-codes-client-vbs

This project is a research-oriented client based on the [SecureGame](https://github.com/SamuelTulach/SecureGame) project (MIT licensed). It has been modified and extended for research purposes, specifically focusing on networking improvements, game logic adjustments, and **enclave-based integrity enforcement**.

---

## ⚖️ License & Attribution
*   **Original Project:** [SecureGame](https://github.com/SamuelTulach/SecureGame/blob/master/LICENSE.txt) by Samuel Tulach.
*   **Documentation Reference:** Enclave commands and implementation details are referenced from the [Microsoft VBS Enclaves Development Guide](https://learn.microsoft.com/en-us/windows/win32/trusted-execution/vbs-enclaves-dev-guide).

---

## VBS Setup Manual

Before executing the research codes, you must configure your Windows environment to support Virtualization-based Security (VBS) enclaves.

### 1. Enable Windows Features
Ensure the following features are enabled via "Turn Windows features on or off":
*   **Windows Hypervisor Platform**
*   **Virtual Machine Platform**

### 2. Enable Test Signing Mode
You must enable test signing to load the enclave.

> [!CAUTION]
> **Warning:** The command below must be executed with **Secure Boot turned OFF** in your BIOS settings.

Open Command Prompt or PowerShell as **Administrator**:
```bash
bcdedit /set testsigning on

---

## Certificate & Enclave Signing

To run the VBS enclave DLL, it must be signed with a specific test certificate.

### 1. Create Test Signing Certificate
Run the following command in **PowerShell (Admin)** to create a certificate named `MyTestEnclaveCert`:

```powershell
New-SelfSignedCertificate -CertStoreLocation Cert:\CurrentUser\My -DnsName "MyTestEnclaveCert" -KeyUsage DigitalSignature -KeySpec Signature -KeyLength 2048 -KeyAlgorithm RSA -HashAlgorithm SHA256 -TextExtension "2.5.29.37={text}1.3.6.1.5.5.7.3.3,1.3.6.1.4.1.311.76.57.1.15,1.3.6.1.4.1.311.97.814040577.346743379.4783502.105532346"

### 2. Install the Certificate

    Press Win + R, type certmgr.msc, and press Enter.

    Go to Personal > Certificates.

    Right-click MyTestEnclaveCert and Copy it.

    Navigate to Trusted Root Certification Authorities > Certificates and Paste it there.

### 3. Sign the VBS Enclave DLL

Use the signtool.exe utility to sign your DLL.

[!IMPORTANT]
Warning: In some cases, signtool is not recognized in PowerShell by default. If so, execute this command within the Windows Kits binary directory:

C:\Program Files (x86)\Windows Kits\10\bin\<your build version>\x64

Run the following command:

```powershell
& .\signtool.exe sign /fd SHA256 /ph /a /n "MyTestEnclaveCert" <location_to_your_dll>
