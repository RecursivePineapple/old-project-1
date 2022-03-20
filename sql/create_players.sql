
drop table if exists player_data;
drop table if exists player;

create table player (
	id uuid not null,

	username varchar not null,
	secret varchar not null,

	deleted boolean default false,

	constraint player_pk primary key(id)
);

create unique index on player(username);

create table player_data (
	player_id uuid not null,

	rel varchar not null,

	entity_id uuid not null,

	constraint player_data_pk primary key (player_id),
	constraint player_data_fk foreign key (player_id) references player(id) on delete no action
);

create index on player_data(player_id, rel);
