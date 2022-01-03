
drop table if exists players;

create table player (
	username varchar not null,
	secret varchar not null,

	world_id uuid,
	entity_id uuid,

	deleted boolean,

	constraint player_world_fk foreign key (world_id) references world(world_id) on delete no action,
	constraint player_entity_fk foreign key (world_id, entity_id) references entity(world_id, id) on delete no action

);

create unique index on player(username);
