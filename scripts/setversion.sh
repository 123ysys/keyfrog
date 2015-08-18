#!/bin/zsh

if [ "${PWD##*/}" = "scripts" ]; then
        cd ..
fi

echo "What is the new version?"
read version

echo "Setting the version to $version"

gsed -i "s/PROJECT_NUMBER.\+=.\+/PROJECT_NUMBER         = $version/" Doxyfile
gsed -i "s/Version: [0-9]\+\.[0-9]\+.*/Version: $version/" keyfrog-1.spec
gsed -i "s/AC_INIT(\[Keyfrog\],\[[0-9]\+\.[0-9]\+.*\])/AC_INIT([Keyfrog],[$version])/" configure.ac
gsed -i "s/setText(\"v. [0-9]\+\.[0-9]\+.*\");/setText(\"v. $version\");/" keyvis/src/org/keyfrog/keyvis/AboutWin.java

echo
echo "Done:"
echo
egrep "PROJECT_NUMBER.+= $version" Doxyfile                                                                     | gsed 's/^ *//g'
egrep "Version: $version" keyfrog-1.spec                                                                                | gsed 's/^ *//g'
egrep "AC_INIT..Keyfrog." configure.ac                                                                                  | gsed 's/^ *//g'
egrep "setText.\"v. $version\".;" keyvis/src/org/keyfrog/keyvis/AboutWin.java   | gsed 's/^ *//g'

