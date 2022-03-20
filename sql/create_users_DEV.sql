
create user "dev-auth-service" with password 'dev-auth-service-pwd';

grant select, insert on table player to "dev-auth-service";

grant select, insert, update, delete on table session to "dev-auth-service";
