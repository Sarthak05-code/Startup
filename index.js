/*

    This below code is done for practice. 

*/

function caller(name , age , password) {
    const nameCheck = name;
    const ageCheck = age;
    const passwordCheck = password;

    return (ageCheck >= 18) ? "Legal" : "Minor";


    
}

const answer = caller("Sarthak" , 20, 123456)

if(answer === "Legal")
    console.log("Valid");
else
    console.log("Not-Valid");
    
    
