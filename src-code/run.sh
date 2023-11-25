#! /usr/bin/env sh

# BY NOW!!
case "$1" in
    -b|--build)
        echo "-> building project"
        ./gradlew build || exit 1 # -q examples:build
    ;;
esac

echo "-> running grades-manager"
# java -jar build/libs/grades-manager-0.1.0-SNAPSHOT.jar
java -cp bin/main/ jh.craft.interpreter.Main


