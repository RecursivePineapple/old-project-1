
#pragma once

#include "Utils/jsonparser.hpp"
#include "Utils/jsonstruct.hpp"

#include "Common/Types.hpp"
#include "Common/WorldTransform.hpp"

namespace jsontypes
{
    template<typename T, typename J>
    struct vector
    {
        typedef linalg::vec<T, 3> ctype;
        typedef jsontypes::array<J> jtype;

        static bool parse(const jsmntok_t *toks, int tok, const char *str, ctype &value)
        {
            typename jtype::ctype a;
            if(!jtype::parse(toks, tok, str, a)) { return false; }
            
            if(a.size() != 3) { return false; }

            value.x = a[0];
            value.y = a[1];
            value.z = a[2];

            return true;
        }

        template<typename stream>
        static stream& emit(stream &s, ctype const& value)
        {
            typename jtype::ctype a {
                static_cast<typename J::ctype>(value.x),
                static_cast<typename J::ctype>(value.y),
                static_cast<typename J::ctype>(value.z)
            };

            return jtype::emit(s, a);
        }
    };

    using location = vector<int, jsontypes::integer>;
    using rotation = vector<float, jsontypes::floating>;
    using scale = vector<float, jsontypes::floating>;

    #define TRANSFORM_FIELDS(X) \
        X(gamestate::Location, location, location) \
        X(gamestate::Rotation, rotation, rotation) \
        X(gamestate::Scale, scale, scale)

    struct transform {
        DECLARE_JSON_STRUCT_BODY(transform, TRANSFORM_FIELDS)

        transform() { }

        transform(
            gamestate::Transform const& transform
        ) : location(transform.m_loc),
            rotation(transform.m_rot),
            scale(transform.m_scale)
        { }

        transform& operator=(gamestate::Transform const& transform)
        {
            this->location = transform.m_loc;
            this->rotation = transform.m_rot;
            this->scale = transform.m_scale;
            return *this;
        }

        operator gamestate::Transform()
        {
            return gamestate::Transform(location, rotation, scale);
        }
    };
}
