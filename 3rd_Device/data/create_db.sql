create database InfectivityDB;
use InfectivityDB;

create table Clients(
	ClientID int not null auto_increment,
	CurrentIP varchar(20),
	MAC varchar(20),
	IsConnected smallint,
	IsTesting smallint,
	InfectivityType smallint,
	Score smallint,
	LastConnected datetime,
	PRIMARY KEY(ClientID)
) ENGINE=InnoDB COLLATE=utf8mb4_general_ci; 
 
create table Platforms(
	PlatformID smallint not null auto_increment,
	Name varchar(20),
	Score smallint,
	PRIMARY KEY(PlatformID)
 )ENGINE=InnoDB COLLATE=utf8mb4_general_ci;
 
create table Categories(
	CategoryID smallint not null auto_increment,
	Name varchar(20),
	Score smallint,
	PRIMARY KEY(CategoryID)
 )ENGINE=InnoDB COLLATE=utf8mb4_general_ci;
 
create table Samples(
	SampleID int primary key not null auto_increment,
	PlatformID smallint not null,
	CategoryID smallint not null,
	Name varchar(30),
	Score smallint,
	CONSTRAINT `fk_sample_platform` 
		FOREIGN KEY (PlatformID) REFERENCES Platforms (PlatformID)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
	CONSTRAINT `fk_sample_category` 
		FOREIGN KEY (CategoryID) REFERENCES Categories (CategoryID)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
)ENGINE=InnoDB COLLATE=utf8mb4_general_ci; 

create table Packages(
	PackageID bigint not null auto_increment,
	SourceIP varchar(20),
	SourcePort int,
	DestinationIP varchar(20),
	DestinationPort int,
	NetworkProtocol smallint,
	TransportProtocol smallint,
	ApplicationProtocol smallint,
	ArriveTime datetime,
	Payload BLOB(500),
	PRIMARY KEY(PackageID)
)ENGINE=InnoDB COLLATE=utf8mb4_general_ci;

create table Tests(
	TestID bigint primary key not null auto_increment,
	ClientID int not null,
	TimeTaken datetime,
	TimeFinished datetime,
	Status smallint,
	CONSTRAINT `fk_test_client` 
		FOREIGN KEY (ClientID) REFERENCES Clients(ClientID)
        ON DELETE CASCADE
        ON UPDATE RESTRICT
)ENGINE=InnoDB COLLATE=utf8mb4_general_ci;

create table TestResults(
	TestID bigint not null,
	SampleID int not null,
	NumberOfTimes int,
	CONSTRAINT `fk_testresults_test` 
		FOREIGN KEY (TestID) REFERENCES Tests(TestID)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
	CONSTRAINT `fk_testresults_sample` 
		FOREIGN KEY (SampleID) REFERENCES Samples(SampleID)
        ON DELETE CASCADE
        ON UPDATE RESTRICT,
	PRIMARY KEY (TestID,SampleID)
)ENGINE=InnoDB COLLATE=utf8mb4_general_ci;

create table Heuristics(
	HeuristicID bigint not null,
	Name varchar(100),
	Type smallint,
	Requirements varchar(200),
	Path varchar(200),
	PRIMARY KEY (HeuristicID)
)ENGINE=InnoDB COLLATE=utf8mb4_general_ci;