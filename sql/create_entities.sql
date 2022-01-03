
drop table if exists entity;

create table entity (
	world_id uuid not null,
	x integer not null,
	y integer not null,
	z integer not null,
	
	id uuid not null,
	
	type varchar not null,
	
	data json,

	deleted boolean,
	
	primary key(world_id, id)

) partition by hash (world_id);

create index on entity(world_id, x, y, z);

create index on entity(id);
