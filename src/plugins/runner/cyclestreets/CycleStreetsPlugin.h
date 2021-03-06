//
// This file is part of the Marble Virtual Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2013      Mihail Ivchenko <ematirov@gmail.com>
//


#ifndef MARBLE_CYCLESTREETSPLUGIN_H
#define MARBLE_CYCLESTREETSPLUGIN_H

#include "RoutingRunnerPlugin.h"

namespace Marble
{

class CycleStreetsPlugin : public RoutingRunnerPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.marble.CycleStreetsPlugin")
    Q_INTERFACES( Marble::RoutingRunnerPlugin )

public:
    explicit CycleStreetsPlugin( QObject *parent = 0 );

    QString name() const;

    QString guiString() const;

    QString nameId() const;

    QString version() const;

    QString description() const;

    QString copyrightYears() const;

    QVector<PluginAuthor> pluginAuthors() const override;

    virtual RoutingRunner *newRunner() const;

    ConfigWidget* configWidget();

    virtual bool supportsTemplate( RoutingProfilesModel::ProfileTemplate profileTemplate ) const;

};

}

#endif
