This is mostly a stab at what to do for now...

1. Create a .ts file for each language using the command `lupdate qml.qrc -ts [ts file name]`. This has to be redone as new strings to translate become available. The ts filename must be `qml_[language abbreviation].ts`.
2. Translators use `linguist [ts file name]` to fill in translations of the strings.
3. Use `lrelease` on the ts files.

Credits:
DE: Luemmel at discuss.pixls.us
PT: Juliano Serra (Juliano_Serra at discuss.pixls.us)
