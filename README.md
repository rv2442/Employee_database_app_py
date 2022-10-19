# [GUI] Employee Database App Using Python

### This is a GUI Employee Database Application using Python and SQLite having basic functionalities:
>1. create 
>2. modify 
>3. delete individuals 
>4. delete all 
>5. import, and export CSV formatted data

### Technologies used:
>1. Python (Tkinter, Easygui)
>2. SQlite

### Dependencies
> ```pip install -r requirements.txt``` in cmd / terminal(pip3)  
>You need to install sqlite browser on your system and create a table using the query:
```sql
CREATE TABLE "employee_database" (
	"id"	INTEGER,
	"employeeid"	VARCHAR(255) NOT NULL UNIQUE,
	"name"	VARCHAR(255) NOT NULL UNIQUE,
	"email"	VARCHAR(255) NOT NULL UNIQUE,
	"number"	INTEGER NOT NULL UNIQUE,
	"branch"	VARCHAR(255) NOT NULL,
	"salary"	INTEGER NOT NULL,
	PRIMARY KEY("id")
);

```
