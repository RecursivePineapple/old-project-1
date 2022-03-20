
DROP TABLE IF EXISTS session;

CREATE TABLE session (
    session_id uuid NOT NULL PRIMARY KEY,
    player_id uuid NOT NULL,

    client_addr INET,

    host_addr INET
);

create unique index on session(player_id);
