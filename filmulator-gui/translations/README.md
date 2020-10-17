This is mostly a stab at what to do for now...

1. Create a .ts file for each language using the command `lupdate qml.qrc [ts file name, perhaps multiple]`. This has to be redone as new strings to translate become available.
2. Translators use `linguist [ts file name]` to fill in translations of the strings.
3. ??? use `lrelease` something something
