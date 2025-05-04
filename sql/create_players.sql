
DROP TABLE IF EXISTS player_data;
DROP TABLE IF EXISTS player;

CREATE TABLE player (
	id uuid NOT NULL,

	username VARCHAR NOT NULL,
	secret VARCHAR NOT NULL,

	deleted BOOLEAN DEFAULT FALSE,

	CONSTRAINT player_pk PRIMARY KEY(id)
);

CREATE UNIQUE INDEX ON player(username);

CREATE TABLE player_data (
	player_id UUID NOT NULL,

	rel VARCHAR NOT NULL,

	data JSON NOT NULL,

	CONSTRAINT player_data_pk PRIMARY KEY (player_id),
	CONSTRAINT player_data_fk FOREIGN KEY (player_id) REFERENCES player(id) ON DELETE NO ACTION
);

CREATE INDEX ON player_data(player_id, rel);
