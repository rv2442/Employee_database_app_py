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

## Demonstration
### 1st Start
![image](https://user-images.githubusercontent.com/69571769/197363224-351c7cf6-bf4a-41ef-9394-ff242488fa38.png)

### Add Employee
![image](https://user-images.githubusercontent.com/69571769/197363326-3ce6b62b-599b-4a1c-9641-748ec1d85e74.png)
<br><br><br>
![image](https://user-images.githubusercontent.com/69571769/197363345-c281b7fd-674c-4324-8868-e4b21e1ccca7.png)

### Modify Employee Details
![image](https://user-images.githubusercontent.com/69571769/197363440-2f9f5f66-b81b-41a7-bc19-2ed7b9b4684c.png)
<br><br><br>
![image](https://user-images.githubusercontent.com/69571769/197363443-cddf6f84-a1af-4353-a0d1-c5dc19c64a3e.png)

### Selective Delete
![image](https://user-images.githubusercontent.com/69571769/197363479-623db063-a0d8-4093-a243-c551f8d8ef98.png)


### Import Data from csv
![image](https://user-images.githubusercontent.com/69571769/197363248-083bc9c1-537b-4e7d-8371-80cfa8210169.png)
<br><br><br>
![image](https://user-images.githubusercontent.com/69571769/197363259-302bf26f-9d7a-41fd-9521-2274009ed885.png)


### Export Data in csv
![image](https://user-images.githubusercontent.com/69571769/197363512-37f760b5-20eb-4fb8-9711-56c950d552cc.png)
<br><br><br>
![image](https://user-images.githubusercontent.com/69571769/197363548-13968631-7aa0-4747-9f91-7c9833ab36a8.png)

### Delete All
![image](https://user-images.githubusercontent.com/69571769/197363558-03e94035-c5d0-460d-bae9-51c5b8787659.png)
<br><br><br>
![image](https://user-images.githubusercontent.com/69571769/197363573-830fe8cb-ea83-41af-b9f1-b70a0ebf4ed9.png)


