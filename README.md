# keytar-pass - Node module to manage system keychain

This library is based on the open source [keytar](https://github.com/atom/node-keytar) library.  keytar uses *libsecret* though, which depends on X11 (GUI).  In order to run on a headless server, this library uses the [pass](https://www.passwordstore.org/) library.  


A native Node module to get, add, replace, and delete passwords in system's keychain. On macOS the passwords are managed by the Keychain, on Windows they are managed by Credential Vault, and on Linux they're managed by the [pass](https://www.passwordstore.org/) library.  

This 

## Installing

```sh
npm install keytar-pass
```

### On Linux

Currently this library uses `pass` so you may need to install it before running `npm install`.

Depending on your distribution, you will need to run the following command:

* Debian/Ubuntu: `sudo apt install pass`
* Red Hat-based: `sudo yum install pass`
* Arch Linux: `sudo pacman -S pass`
* openSUSE: `sudo zypper in password-store`
* Gentoo: `emerge -av pass`

You'll also need to initialize pass from within your nodejs project.  A very simple init script is as follows:

```js
import { exec } from 'child_process';

if (process.platform != 'win32' && process.platform != 'darwin') {
    log.info(`Detected current OS: ${process.platform}.  Initializing pass library for password storage.`);
    exec('pass', (err, stdout, stderr) => {
        if (stdout.startsWith('Password Store')) {
            log.info('Pass library already initialized.');
        } else if (stdout.includes('Error: password store is empty. Try "pass init".')) { 
            // initialize pass
            exec(`cat >keygenInfo <<EOF
                    %echo Generating a basic OpenPGP key
                    %no-protection
                    Key-Type: DSA
                    Key-Length: 1024
                    Subkey-Type: ELG-E
                    Subkey-Length: 1024
                    Name-Real: YOUR NAME HERE
                    Name-Comment: Coolness 123
                    Name-Email: you@domain.com
                    Expire-Date: 0
                    # Do a commit here, so that we can later print "done" :-)
                    %commit
                    %echo done
                EOF`);
            exec('gpg --batch --generate-key keygenInfo');
            exec('pass init you@domain.com', (error, stdout, stderr) => {
                if (error) {
                    log.error(`Error initializing pass: ${error.message}`);
                }
                if (stderr) {
                    log.error(`Error output (stderr) while initializing pass: ${stderr}`);
                }
                log.info(`pass init: output: ${stdout}`);
            });

        } else if (stdout.includes('No such file or directory')) {
            log.error('Using a linux system, but the pass library does not appear to be installed.  Try running "sudo apt install pass".');
        }
    });
}
```

## Building

  * Clone the repository
  * Run `npm install`
  * Run `npm test` to run the tests


## Docs

```javascript
const keytar = require('keytar-pass')

// OR
import keytar from 'keytar-pass';
```

Every function in keytar-pass is asynchronous and returns a promise. The promise will be rejected with any error that occurs or will be resolved with the function's "yields" value.

### getPassword(service, account)

Get the stored password for the `service` and `account`.

`service` - The string service name.

`account` - The string account name.

Yields the string password or `null` if an entry for the given service and account was not found.

### setPassword(service, account, password)

Save the `password` for the `service` and `account` to the keychain. Adds a new entry if necessary, or updates an existing entry if one exists.

`service` - The string service name.

`account` - The string account name.

`password` - The string password.

Yields nothing.

### deletePassword(service, account)

Delete the stored password for the `service` and `account`.

`service` - The string service name.

`account` - The string account name.

Yields `true` if a password was deleted, or `false` if an entry with the given service and account was not found.

### findCredentials(service)

Find all accounts and password for the `service` in the keychain.

`service` - The string service name.

Yields an array of `{ account: 'foo', password: 'bar' }`.

### findPassword(service)

Find a password for the `service` in the keychain. This is ideal for scenarios where an `account` is not required.

`service` - The string service name.

Yields the string password, or `null` if an entry for the given service was not found.

