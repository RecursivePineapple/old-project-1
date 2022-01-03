
drop table if exists world;

create table world (
	world_id uuid not null,
	parent_world_id uuid,

	type varchar not null,
	
	deleted boolean,
	
	primary key(world_id),

	constraint world_parent_fk foreign key (parent_world_id) references world(world_id)
		on delete no action
		on update cascade
);
