var docRoot = null;
function getDocRoot(item) {
    if (!docRoot) {
        docRoot = item;
        while (docRoot.parent) //exists
        {
            docRoot = docRoot.parent;
        }
    }
    return docRoot;
}
