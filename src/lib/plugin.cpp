// Copyright (c) 2022 Manuel Schneider
#include "plugin.h"

extern albert::PluginSpec *current_spec_in_construction;

albert::Plugin::Plugin() : spec(*current_spec_in_construction){}

QString albert::Plugin::id() const
{
    return spec.id;
}
