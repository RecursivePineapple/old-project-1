
drop table if exists entity;

create table entity (
	world_id uuid not null,
	x integer not null,
	y integer not null,
	z integer not null,
	
	id uuid not null,
	
	type varchar not null,
	
	data json,

	deleted boolean default false,
	
	primary key(id)

);

create index on entity(world_id);

create index on entity(x, y, z);

create unique index on entity(id);
