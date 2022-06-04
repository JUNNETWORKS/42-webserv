grammar configuration;

config: server (NEWLINE server)*;
server: 'server' '{' server_directive+ '}';
server_directive:
	listen_directive
	| servername_directive
	| location_directive
	| location_back_directive;

listen_directive: 'listen' WHITESPACE NUMBER END_DIRECTIVE;
servername_directive:
	'server_name' WHITESPACE ((DOMAIN_NAME | IP_ADDR) WHITESPACE)+ END_DIRECTIVE;
location_directive:
	'location' PATH '{' directive_in_location+ '}';
location_back_directive:
	'location_back' PATH '{' directive_in_location+ '}';
directive_in_location:
	allow_method_directive
	| client_max_body_size_directive
	| root_directive
	| index_directive
	| autoindex_directive
	| is_cgi_directive
	| return_directive;

allow_method_directive:
	'allow_method' WHITESPACE METHOD (WHITESPACE METHOD)* END_DIRECTIVE;
client_max_body_size_directive:
	'client_max_body_size' WHITESPACE NUMBER END_DIRECTIVE;
root_directive: 'root' WHITESPACE PATH END_DIRECTIVE;
index_directive:
	'index' WHITESPACE PATH (WHITESPACE PATH)* END_DIRECTIVE;
autoindex_directive:
	'autoindex' WHITESPACE ON_OFF END_DIRECTIVE;
is_cgi_directive: 'is_cgi' WHITESPACE ON_OFF END_DIRECTIVE;
return_directive: 'return' WHITESPACE URL;

ON_OFF: 'on' | 'off';
METHOD: 'GET' | 'POST' | 'DELETE';
PATH: (.*? '/')? (.+?);
URL: ('http' | 'https') '://' DOMAIN_NAME ('/');
DOMAIN_NAME: DOMAIN_LABEL ('.' DOMAIN_LABEL)*;
IP_ADDR: NUMBER+ '.' NUMBER+ '.' NUMBER+ '.' NUMBER+;
DOMAIN_LABEL: (ALPHABET | NUMBER)+
	| (ALPHABET | NUMBER)+ (ALPHABET | NUMBER | HYPHEN)* (
		ALPHABET
		| NUMBER
	)+;
NEWLINE: '\n';
WHITESPACE: ' ';
END_DIRECTIVE: ';';
ALPHABET: 'a' ..'z' | 'A' ..'Z';
NUMBER: ('0' ..'9')+;
HYPHEN: '-';
