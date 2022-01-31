
#pragma once

#ifndef QUERY_FACTORY_ACCESS
#define QUERY_FACTORY_ACCESS private:
#endif

#ifndef QUERY_MEMBER_ACCESS
#define QUERY_MEMBER_ACCESS private:
#endif

#define CAT(x, y) CAT_(x, y)
#define CAT_(x,y) x ## y

#define COMMA() ,

#define EMPTY()
#define DEFER(id) id EMPTY()

#define CHAIN_COMMA(chain) CAT(CHAIN_COMMA_0 chain, _END)
#define CHAIN_COMMA_0(x) x CHAIN_COMMA_1
#define CHAIN_COMMA_1(x) DEFER(COMMA)() x CHAIN_COMMA_2
#define CHAIN_COMMA_2(x) DEFER(COMMA)() x CHAIN_COMMA_1
#define CHAIN_COMMA_0_END
#define CHAIN_COMMA_1_END
#define CHAIN_COMMA_2_END

#define _PREPARED_T_PARAM_CHAIN_X(type, name) (dbwrapper::libpq::basic_dtype::type)
#define _QUERY_PARAM_IDX_CHAIN_X(type, name) (name)

#define _QUERY_PARAM(p) dbwrapper::libpq::param(param_idxs::p)

#define _QUERY_X(name, params, query) \
    QUERY_FACTORY_ACCESS \
    typedef dbwrapper::libpq::prepared_query2<CHAIN_COMMA(params(_PREPARED_T_PARAM_CHAIN_X))> name ## _t; \
    static std::shared_ptr<name ## _t> get_ ## name(std::shared_ptr<dbwrapper::libpq::database> const& db) { \
        struct param_idxs { enum { CHAIN_COMMA(params(_QUERY_PARAM_IDX_CHAIN_X)) }; }; \
        query(_QUERY_PARAM) \
        return db->prepare2<CHAIN_COMMA(params(_PREPARED_T_PARAM_CHAIN_X))>(sql, #name); \
    } \
    QUERY_MEMBER_ACCESS \
    std::shared_ptr<name ## _t> m_ ## name; \
    public:

#define _QUERY_ASSIGN_X(name, params, query) m_ ## name = get_ ## name(db);

#define _INVOKE_PARAMS_X(type, name) (dbwrapper::libpq::conversion<dbwrapper::libpq::basic_dtype::type>::ctype name)
#define _INVOKE_PARAMS2_X(type, name) (name)

#define _QUERY_INVOKE_X(name, params, query) \
    dbwrapper::libpq::result name(CHAIN_COMMA(params(_INVOKE_PARAMS_X))) { \
        return m_ ## name->exec(CHAIN_COMMA(params(_INVOKE_PARAMS2_X))); \
    }

#define QUERY_STRUCT_DECL(name, QUERY_LIST) \
    struct name { \
        QUERY_FACTORY_ACCESS \
        QUERY_LIST(_QUERY_X) \
        name() { } \
        name(std::shared_ptr<dbwrapper::libpq::database> const& db) { \
            QUERY_LIST(_QUERY_ASSIGN_X) \
        } \
        QUERY_LIST(_QUERY_INVOKE_X) \
    };
