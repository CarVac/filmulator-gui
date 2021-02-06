This is mostly a stab at what to do for now...

1. Create a .ts file for each language using the command `lupdate qml.qrc -ts [ts file name]`. This has to be redone as new strings to translate become available. The ts filename must be `qml_[language abbreviation].ts`. When the translation is complete, run the same command with the argument -no-obsolete to drop obsolete and vanished strings.
2. Translators use `linguist [ts file name]` to fill in translations of the strings.
3. Use `lrelease` on the ts files.

Testing (on Linux): LANG=de_DE.UTF-8 filmulator-gui

Credits:
de_DE: Luemmel at discuss.pixls.us
pt_BR: Juliano Serra (Juliano_Serra at discuss.pixls.us)
