#include "notemanager.h"

NoteManager* NoteManager::instance = nullptr;

NoteManager::NoteManager() : QObject()
{
    filename = "C:\\Users\\Paul\\Documents\\GI02\\LO21\\Projet\\PluriNotes\\save.xml";
}

NoteManager::~NoteManager()
{

}

NoteManager& NoteManager::getInstance()
{
    if(NoteManager::instance == nullptr)
        NoteManager::instance = new NoteManager();

    return *NoteManager::instance;
}

void NoteManager::deleteInstance()
{
    if(NoteManager::instance != nullptr)
        delete NoteManager::instance;

    NoteManager::instance = nullptr;
}


void NoteManager::nouvelleNote(const QString& id, const QDateTime& crea, const QDateTime& modif, bool archive, bool corbeille)
{
    Note* nouvelle = new Note(id, crea, modif, archive, corbeille);
    notes.push_back(nouvelle);
}

void NoteManager::saveAll() const
{
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        throw NoteException(QString("Erreur création du fichier de sauvegarde"));

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Sauvegarde");

    for(auto it = notes.begin(); it != notes.end(); it++)
    {
        Note& actu = **it;

        stream.writeStartElement("Note");
        stream.writeTextElement("id", actu.getId());
        stream.writeTextElement("Creation", actu.getCreation().toString());
        stream.writeTextElement("Modification", actu.getModif().toString());

        stream.writeTextElement("Archive", QString::number(actu.isArchive()));
        stream.writeTextElement("Corbeille", QString::number(actu.isInCorbeille()));

        for(Note::iterator itVersion = actu.begin(); itVersion != actu.end(); ++itVersion)
        {
            (*itVersion).writeInFile(stream);
        }
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();
    file.close();
}

void NoteManager::load()
{
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw NoteException(QString("Erreur lecture du fichier de sauvegarde"));

    QXmlStreamReader stream(&file);

    while(!stream.atEnd() && !stream.hasError())
    {
        QXmlStreamReader::TokenType token = stream.readNext();

        if(token == QXmlStreamReader::StartDocument) continue; // Si c'est la balise de début de document

        if(token == QXmlStreamReader::StartElement) // Si c'est une balise de début d'élément
        {
            if(stream.name() == "Sauvegarde") continue; // Si c'est la balise qui englobe tout

            if(stream.name() == "Note") // Si c'est le début d'une note
            {
                while(stream.readNext() != QXmlStreamReader::StartElement);
                stream.readNext();
                // On récupère toutes les infos de la note

                QString id = stream.text().toString();

                while(stream.readNext() != QXmlStreamReader::StartElement);
                stream.readNext();

                QDateTime creation = QDateTime::fromString(stream.text().toString());

                while(stream.readNext() != QXmlStreamReader::StartElement);
                stream.readNext();

                QDateTime modif = QDateTime::fromString(stream.text().toString());

                while(stream.readNext() != QXmlStreamReader::StartElement);
                stream.readNext();

                bool archive = stream.text().toString().toInt();

                while(stream.readNext() != QXmlStreamReader::StartElement);
                stream.readNext();

                bool corbeille = stream.text().toString().toInt();
                nouvelleNote(id, creation, modif, archive, corbeille);
            }

            if(stream.name() == "Version")
            {
                // On recupère le type de la version
                QString type = stream.readElementText();

                // Cette version appartient a la dernière note ajoutée
                // On laisse la note s'occuper d'ajouter la version
                notes.back()->ajouterVersion(type, stream);
            }
        }
    }
}

const Note& NoteManager::find(const QString &id) const
{
    for(QVector<Note*>::const_iterator it = notes.begin(); it != notes.end(); it++)
    {
        if((*it)->getId() == id)
            return **it;
    }

    throw NoteException(QString("Note non trouvée"));
}








