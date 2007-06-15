//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2006-2007 Torsten Rahn <tackat@kde.org>"
// Copyright 2007      Inge Wallin  <ingwa@kde.org>"
//

#include "placemarkmanager.h"

#include <QtCore/QDebug>
#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtXml/QXmlInputSource>
#include <QtXml/QXmlSimpleReader>

#include "xmlhandler.h"
#include "kml/KMLDocument.h"


PlaceMarkManager::PlaceMarkManager()
{
    m_placecontainer = new PlaceContainer();

    addPlaceMarkFile( "cityplacemarks" );
    addPlaceMarkFile( "baseplacemarks" );
    addPlaceMarkFile( "elevplacemarks" );

#ifdef KML_GSOC
    /*
     * Sample code to test parsing of kml document
     */
    QString defaultSrcName = KAtlasDirs::path( "placemarks/baseplacemarks.kml" );

    if ( QFile::exists( defaultSrcName ) ) {
        QFile sourceFile( defaultSrcName );

        if ( sourceFile.open( QIODevice::ReadOnly ) ) {
            KMLDocument document;
            document.load( sourceFile );
        }
    }
#endif
}


void PlaceMarkManager::addPlaceMarkFile( const QString& filepath )
{

    QString  defaultcachename;
    QString  defaultsrcname;
    QString  defaulthomecache;

    if ( !filepath.contains( "\\" && !filepath.contains( '/' ) ) ) {
        defaultcachename = KAtlasDirs::path( "placemarks/" + filepath + ".cache" );
        defaultsrcname   = KAtlasDirs::path( "placemarks/" + filepath + ".kml");
        defaulthomecache = KAtlasDirs::localDir() + "/placemarks/" + filepath + ".cache";
    }
    else
        return;

    if ( QFile::exists( defaultcachename ) ) {
        qDebug() << "Loading Default Placemark Cache File:" + defaultcachename;

        bool       cacheoutdated = false;
        QDateTime  sourceLastModified;
        QDateTime  cacheLastModified;

        if ( QFile::exists( defaultsrcname ) ) {
            sourceLastModified = QFileInfo( defaultsrcname ).lastModified();
            cacheLastModified  = QFileInfo( defaultcachename ).lastModified();

            if ( cacheLastModified < sourceLastModified )
                cacheoutdated = true;
        }

        bool  loadok = false;

        if ( cacheoutdated == false )
            loadok = loadFile( defaultcachename, m_placecontainer );

        if ( loadok == true )
            return;
    }

    qDebug( "No recent Default Placemark Cache File available!" );

    if ( QFile::exists( defaultsrcname ) ) {
        PlaceContainer  *importcontainer = new PlaceContainer();

        importKml( defaultsrcname, importcontainer );
        saveFile( defaulthomecache, importcontainer );

        *m_placecontainer << *importcontainer;
    }
    else {
        qDebug() << "No Default Placemark Source File!";
    }
}


void PlaceMarkManager::loadKml( const QString& filename )
{
    // This still is buggy and needs a lot of work as does the concept
    // as a whole ...

    // PlaceContainer* tmp = m_placecontainer;
    m_placecontainer -> clear();
    // tmp -> deleteAll();
    // delete tmp;

    importKml( filename, m_placecontainer );
}

void PlaceMarkManager::importKml( const QString& filename, PlaceContainer* placecontainer )
{

    KAtlasXmlHandler handler( placecontainer );

    QFile file( filename );

    // gzip reader:
#if 0
      QDataStream dataIn(&file);
      QByteArray compByteArray;
      dataIn >> compByteArray;
      QByteArray xmlByteArray = qUncompress(compByteArray);
      QString xmlString = QString::fromUtf8(xmlByteArray.data(), xmlByteArray.size());
      QXmlInputSource source;
      source.setData(xmlString);
#endif
    QXmlInputSource   source( &file );

    QXmlSimpleReader  reader;
    reader.setContentHandler( &handler );
    reader.parse( source );
}

void PlaceMarkManager::saveFile( const QString& filename, PlaceContainer* placecontainer )
{
    if ( QDir( KAtlasDirs::localDir() + "/placemarks/" ).exists() == false )
        ( QDir::root() ).mkpath( KAtlasDirs::localDir() + "/placemarks/" );

    QFile  file( filename );
    file.open(QIODevice::WriteOnly);
    QDataStream  out(&file);

    // Write a header with a "magic number" and a version
    // out << (quint32)0xA0B0C0D0;
    out << (quint32)0x31415926;
    out << (qint32)005;

    out.setVersion(QDataStream::Qt_4_0);

    double  lng;
    double  lat;

    PlaceContainer::const_iterator  it;

    for ( it=placecontainer->constBegin();
          it != placecontainer->constEnd();
          it++ )
    {
        out << (*it) -> name();
        (*it) -> coordinate(lng, lat);

        out << lng << lat;
        out << QString( (*it) -> role() );
        out << QString( (*it) -> description() );
        out << QString( (*it) -> countryCode() );
        out << (qint32)(*it) -> popidx();
        out << (qint32)(*it) -> symbol();
        out << (qint32)(*it) -> population();
    }
}


bool PlaceMarkManager::loadFile( const QString& filename,
                                 PlaceContainer* placecontainer )
{
    QFile  file( filename );
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    // Read and check the header
    quint32 magic;
    in >> magic;
    if (magic != 0x31415926) {
        qDebug( "Bad file format!" );
        return false;
    }
    // Read the version
    qint32 version;
    in >> version;
    if (version < 005) {
        qDebug( "Bad file - too old!" );
        return false;
    }
    /*
      if (version > 002) {
      qDebug( "Bad file - too new!" );
      return;
      }
    */

    in.setVersion(QDataStream::Qt_4_0);

    // Read the data
    double    lng;
    double    lat;
    QString  tmpstr;
    qint32   a;

    PlaceMark  *mark;

    while ( !in.atEnd() ) {
        mark = new PlaceMark();

        in >> tmpstr;
        mark -> setName( tmpstr );
        in >> lng >> lat;
        mark -> setCoordinate(lng, lat);
        in >> tmpstr;
        mark -> setRole( tmpstr.at(0) );
        in >> tmpstr;
        mark -> setDescription( tmpstr );
        in >> tmpstr;
        mark -> setCountryCode( tmpstr );
        in >> a;
        mark -> setPopidx( a );
        in >> a;
        mark -> setSymbol( a );
        in >> a;
        mark -> setPopulation( a );

        placecontainer -> append( mark );
    }

    return true;
}
