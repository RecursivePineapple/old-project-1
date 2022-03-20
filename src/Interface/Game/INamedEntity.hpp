#pragma once

#include "Utils/jsonuuid.hpp"

#include "Common/Types.hpp"
#include "Common/WorldTransform.hpp"
#include "Common/Event.hpp"
#include "Common/ConnectionContext.hpp"

namespace gamestate
{
    struct INamedEntity
    {
        virtual ~INamedEntity() { }

        typedef WorldTransform TransformType;

        virtual std::string EntityType() = 0;

        virtual bool LoadState(CR<jsontypes::span_t> msg) = 0;
        virtual std::string SaveState() = 0;
        
        virtual bool ShouldSave() { return true; }

        virtual bool IsActive() { return true; }

        virtual void BeginPlay() { }
        virtual void EndPlay() { }

        virtual void OnMessage(ConnectionContext *sender, CR<std::string> action, std::optional<jsontypes::span_t> data) = 0;

        virtual Event<INamedEntity*>& OnMove() = 0;
        virtual Event<INamedEntity*>& OnStateChanged() = 0;

        PROPERTY(uuid, Id)
        PROPERTY(TransformType, Location)

    };
}
