
DROP TABLE IF EXISTS session;

CREATE TABLE session (
    session_id uuid NOT NULL PRIMARY KEY,
    player_id uuid NOT NULL,

    client_addr INET,

    host_addr uuid NOT NULL,

    constraint player_id_fk FOREIGN KEY(player_id) REFERENCES player(id)
);

CREATE UNIQUE INDEX ON session(player_id);
