#! /usr/bin/env sh

set -e
# BY NOW!!
case "$1" in
    -b|--build)
        echo "-> building jlox"
        ./scripts/ast_generator.py
        ./gradlew build
        shift
    ;;
esac

echo "-> running jlox"
java -jar build/libs/*.jar $*


