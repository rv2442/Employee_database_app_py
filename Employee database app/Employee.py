from tkinter import Tk, Button, PhotoImage, Label, LabelFrame, W,E,N,S,Entry,END,StringVar,Scrollbar,Toplevel
from tkinter import ttk
import sqlite3
import easygui as explorer

class Database:
    db_filename = r"C:\Users\Rahul\OneDriveSky\Desktop\PROJ FILES\Python Tkinter\Employee database app\database.db"

    def __init__(self,root):
        self.root=root
        self.create_gui()
        ttk.style = ttk.Style()
        ttk.style.configure("Treeview", font=('helvetica',10))
        ttk.style.configure("Treeview.Heading", font=('helvetica',12, 'bold'))
        
    def create_gui(self):
        self.create_left_icon()
        self.create_label_frame()
        self.create_message_area()
        self.create_tree_view()
        self.create_scrollbar()
        self.create_bottom_buttons()
        self.view_contacts()
        
    def execute_db_query(self, query, parameters=()):
        with sqlite3.connect(self.db_filename) as conn:
            print(conn)
            print('You have successfully connected to the Database')
            cursor = conn.cursor()
            query_result = cursor.execute(query, parameters)
            conn.commit()
        return query_result

    def create_left_icon(self):
        photo=PhotoImage(file=r"C:\Users\Rahul\OneDriveSky\Desktop\PROJ FILES\Python Tkinter\Employee database app\employee_icon.png")
        label=Label(image=photo)
        label.image=photo
        label.grid(row=0,column=0)

    def create_label_frame(self):
        labelframe = LabelFrame(self.root, text='Add Employee to Database',bg="sky blue",font="helvetica 10")
        labelframe.grid(row=0, column=1, padx=8, pady=8, sticky='ew')
        Label(labelframe, text='Employee ID:',bg="blue",fg="white").grid(row=1, column=1, sticky=W,  pady=2,padx=15)
        self.idfield = Entry(labelframe)
        self.idfield.grid(row=1, column=2, sticky=W, padx=5, pady=2)
        Label(labelframe, text='Name:',bg="green",fg="white").grid(row=2, column=1, sticky=W, pady=2,padx=15)
        self.namefield = Entry(labelframe)
        self.namefield.grid(row=2, column=2, sticky=W, padx=5, pady=2)
        Label(labelframe, text='Email:',bg="brown",fg="white").grid(row=3, column=1, sticky=W,  pady=2,padx=15)
        self.emailfield = Entry(labelframe)
        self.emailfield.grid(row=3, column=2, sticky=W, padx=5, pady=2)
        Label(labelframe, text='Number:',bg="black",fg="white").grid(row=4, column=1, sticky=W,  pady=2,padx=15)
        self.numfield = Entry(labelframe)
        self.numfield.grid(row=4, column=2, sticky=W, padx=5, pady=2)
        Label(labelframe, text='Branch:',bg="orange",fg="white").grid(row=5, column=1, sticky=W,  pady=2,padx=15)
        self.branchfield = Entry(labelframe)
        self.branchfield.grid(row=5, column=2, sticky=W, padx=5, pady=2)
        Label(labelframe, text='Salary:',bg="purple",fg="white").grid(row=6, column=1, sticky=W,  pady=2,padx=15)
        self.salaryfield = Entry(labelframe)
        self.salaryfield.grid(row=6, column=2, sticky=W, padx=5, pady=2)
        Button(labelframe, text='Add Contact', command=self.on_add_contact_button_clicked,bg="blue",fg="white").grid(row=7, column=2, sticky=E, padx=5, pady=5)
        
    def on_add_contact_button_clicked(self):
        self.add_new_contact()
        
    def on_delete_selected_button_clicked(self):
        self.message['text'] = ''
        try:
            self.tree.item(self.tree.selection())['values'][0]
        except IndexError as e:
            self.message['text'] = 'Please select a registry to delete'
            return
        self.delete_contacts()
        
    def on_modify_selected_button_clicked(self):
        self.message['text'] = ''
        try:
            self.tree.item(self.tree.selection())['values'][0]

        except IndexError as e:
            self.message['text'] = 'Please select a registry to modify'
            return
        self.open_modify_window()    

    def create_message_area(self):
        self.message = Label(text='', fg='red')
        self.message.grid(row=3, column=1, sticky=W)

    def create_tree_view(self):
        self.tree = ttk.Treeview(height=25, columns=("name","email","number","branch","salary"),style='Treeview')
        self.tree.grid(row=6, column=0, columnspan=3)
        self.tree.heading('#0', text='Employee ID', anchor=W)
        self.tree.heading("name", text='Name', anchor=W)
        self.tree.heading("email", text='Email Address', anchor=W)
        self.tree.heading("number", text='Contact Number', anchor=W)
        self.tree.heading("branch", text='Branch', anchor=W)
        self.tree.heading("salary", text='Salary', anchor=W)
    
    def create_scrollbar(self):
        self.scrollbar = Scrollbar(orient='vertical',command=self.tree.yview)
        self.scrollbar.grid(row=6,column=6,rowspan=10,sticky='sn')
        
    def create_bottom_buttons(self):
        Button(text='Delete Selected', command=self.on_delete_selected_button_clicked,bg="red",fg="white").grid(row=8, column=0, sticky=W,pady=10,padx=20)
        Button(text='Delete All', command=self.delete_all_data_buttonpressed,bg="red",fg="black").grid(row=8, column=0, sticky=W,pady=10,padx=113)
        Button(text='Modify Selected', command=self.on_modify_selected_button_clicked,bg="purple",fg="white").grid(row=8, column=1, sticky=W)
        Button(text='Export data',command=self.export_data,bg="black",fg="white").grid(row=8,column=2,sticky=W)
        Button(text='Import data',command=self.import_data,bg="blue",fg="white").grid(row=8,column=1,sticky=E,padx=2)
        
    def import_data(self):
        file = explorer.fileopenbox()
        data=open(file,"rt")
        datalist=(data.read()).split("\n")
        data.close()
        for a in datalist:
            if a!="":
                b=a.split(",")
                query = 'INSERT INTO employee_database VALUES(NULL,?,?,?,?,?,?)'
                self.execute_db_query(query,b)
        self.view_contacts()        
        self.message['text']= 'Data imported' 
   
    def export_data(self):
        query = 'SELECT * FROM employee_database ORDER BY employeeid desc'
        contact_entries = self.execute_db_query(query)
        datalist_export=[]
        for row in contact_entries:
            data_list="{},{},{},{},{},{}\n".format(row[1],row[2],row[3],row[4],row[5],row[6])
            datalist_export.append(data_list)
        if(datalist_export!=[]):
            file = explorer.diropenbox() + r"\Exported_Data_DBapp.txt"
            data=open(file,"w+")
            #c=open(r'C:\Users\Rahul\OneDrive - Sky University\Desktop\PROJ FILES\Python Tkinter\Employee database app\Exported database Records\Employee_database_export.txt',"w+")
            #c=open(r'C:\Users\Rahul\OneDrive - Sky University\Desktop\PROJ FILES\Python Tkinter\Employee database app\Exported database Records\Employee_database_export.txt',"a+")
            data.write("")
            data=open(file,"a+")
            for b in datalist_export:
                data.write(b)
            data.close()
            #path=r"Desktop\PROJ FILES\Python Tkinter\Employee database app\Exported database Records"
            self.message['text'] = 'Data Exported to {}'.format(file)
        else:
            self.message['text'] = 'No data to export'
        
            
            
        
        
    def add_new_contact(self):
        if self.new_contacts_validated():
            query = 'INSERT INTO employee_database VALUES(NULL,?,?,?,?,?,?)'
            parameters = (self.idfield.get(),self.namefield.get(),self.emailfield.get(), self.numfield.get(),self.branchfield.get(),self.salaryfield.get())
            self.execute_db_query(query, parameters)
            self.message['text'] = 'New Contact {} added'.format(self.namefield.get())
            self.namefield.delete(0, END)
            self.emailfield.delete(0, END)
            self.numfield.delete(0, END)
            self.idfield.delete(0, END)
            self.branchfield.delete(0, END)
            self.salaryfield.delete(0, END)
            self.view_contacts()

        else:
            self.message['text'] = 'Any field cannot be blank'
            self.view_contacts()

    def new_contacts_validated(self):
        return len(self.namefield.get()) != 0 and len(self.emailfield.get()) != 0 and len(self.numfield.get()) != 0 and len(self.idfield.get()) !=0 and len(self.branchfield.get()) !=0 and len(self.salaryfield.get()) !=0
    
    def view_contacts(self):
        items = self.tree.get_children()
        for item in items:
            self.tree.delete(item)
        query = 'SELECT * FROM employee_database ORDER BY employeeid desc'
        contact_entries = self.execute_db_query(query)
        for row in contact_entries:
            self.tree.insert('', 0, text=row[1], values=(row[2],row[3],row[4],row[5],row[6]))
            
    def delete_contacts(self):
        self.message['text'] = ''
        employeeid = self.tree.item(self.tree.selection())['text']
        name=self.tree.item(self.tree.selection())['values'][0]
        query = 'DELETE FROM employee_database WHERE employeeid = ?'
        self.execute_db_query(query, (employeeid,))
        self.message['text'] = 'Employee {} removed from database'.format(name)
        self.view_contacts()
    
    def delete_all_data_buttonpressed(self):

        self.transient = Toplevel()
        self.transient.geometry("450x163")
        self.transient.resizable(width=False, height=False)
        self.transient.title('Alert')
        labelframe_Alert = LabelFrame(self.transient,font="helvetica 12")
        labelframe_Alert.grid(row=0, column=1, padx=8,sticky='ew')
        photo=PhotoImage(file="Alert_icon.png")
        label=Label(labelframe_Alert,image=photo)
        label.image=photo
        label.grid(row=1,column=0,padx=15)
        Label(labelframe_Alert, text='Are you sure you wish to delete all data?',font="helvetica 12").grid(row=1, column=1,sticky=W,pady=20,padx=25)
        Button(labelframe_Alert, text='    Yes    ',command=self.delete_all_data).grid(row=3,column=1,sticky=W,pady=30,padx=45)
        Button(labelframe_Alert, text='    No     ', command=self.transient.destroy).grid(row=3,column=1,sticky=W,pady=30,padx=130)
        
    def delete_all_data(self):
        query='DELETE FROM [employee_database]'
        self.execute_db_query(query)
        self.message['text']='All data deleted'
        self.transient.destroy()
        self.view_contacts()
        
          
    def open_modify_window(self):
        ID = self.tree.item(self.tree.selection())['text']
        name = self.tree.item(self.tree.selection())['values'][0]
        old_email = self.tree.item(self.tree.selection())['values'][1]
        old_number = self.tree.item(self.tree.selection())['values'][2]
        old_branch = self.tree.item(self.tree.selection())['values'][3]
        old_salary = self.tree.item(self.tree.selection())['values'][4]
        self.transient = Toplevel()
        self.transient.geometry("380x210")
        self.transient.resizable(width=False, height=False)
        self.transient.title('Update Contact')
        labelframe2 = LabelFrame(self.transient, text='Update Database',bg="sky blue",font="helvetica 12")
        labelframe2.grid(row=0, column=1, padx=8,sticky='ew')
        
        Label(labelframe2, text='Employee ID :',bg="blue",fg="white").grid(row=0, column=1,sticky=W,pady=2,padx=35)
        Entry(labelframe2, textvariable=StringVar(self.transient, value=ID), state='readonly').grid(row=0, column=2,pady=2,padx=45)
        
        Label(labelframe2, text='Name :',bg="red",fg="white").grid(row=1, column=1,sticky=W,pady=2,padx=35)
        Entry(labelframe2, textvariable=StringVar(self.transient, value=name), state='readonly').grid(row=1, column=2,pady=2,padx=45)
        
        Label(labelframe2, text='Email :',bg="green",fg="white").grid(row=2, column=1,sticky=W,pady=2,padx=35)
        new_email=Entry(labelframe2, textvariable=StringVar(self.transient, value=old_email))
        new_email.grid(row=2, column=2,pady=2,padx=45)
        
        Label(labelframe2, text='Number :',bg="black",fg="white").grid(row=3, column=1,sticky=W,pady=2,padx=35)
        new_number=Entry(labelframe2, textvariable=StringVar(self.transient, value=old_number))
        new_number.grid(row=3, column=2,pady=2,padx=45)
        
        Label(labelframe2, text='Branch :',bg="orange",fg="white").grid(row=4, column=1,sticky=W,pady=2,padx=35)
        new_branch=Entry(labelframe2, textvariable=StringVar(self.transient, value=old_branch))
        new_branch.grid(row=4, column=2,pady=2,padx=45)
        
        Label(labelframe2, text='Salary :',bg="purple",fg="white").grid(row=5, column=1,sticky=W,pady=2,padx=35)
        new_salary=Entry(labelframe2, textvariable=StringVar(self.transient, value=old_salary))
        new_salary.grid(row=5, column=2,pady=2,padx=45)
        

        
        Button(labelframe2, text='Update Contact',bg="blue",fg="white", command=lambda: self.update_contacts(new_email.get(),new_number.get(),new_branch.get(),new_salary.get(),ID,name)).grid(row=6, column=2,pady=2)


        self.transient.mainloop()
        
    def update_contacts(self,newemail,newnumber,newbranch,newsalary,ID,name):
        query = 'UPDATE employee_database SET email=?, number=?, branch=?, salary=?  WHERE employeeid =? AND name =?' 
        parameters = (newemail,newnumber,newbranch,newsalary,ID,name)
        self.execute_db_query(query, parameters)
        self.transient.destroy()
        self.message['text'] = 'Data of {} modified'.format(name)
        self.view_contacts()

if __name__ == '__main__':
    root= Tk()
    root.title('Employee Database')
    root.geometry("1225x825")
    root.resizable(width=False, height=False)
    application= Database(root)
    root.mainloop()