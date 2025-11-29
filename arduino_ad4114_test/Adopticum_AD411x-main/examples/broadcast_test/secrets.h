// secrets.h
// Store secret values into this file, like passwords etc.
// DO NOT COMMIT SECRETS TO VERSION CONTROL REPOSITORY!
//
// Run this command to prevent further commits of this file.
//     $ git update-index --assume-unchanged platformio/src/secrets.h
//
// An alternative is to add secrets.h to gitignore file, but
// that will not work when the file already exists in the repo.
// Also we __want__ to commit the structure of this file,
// just not the secret values.

namespace secrets {
	static const char wifi_ssid[] = "Wifi name";
	static const char wifi_password[] = "Wifi password"; 
}
