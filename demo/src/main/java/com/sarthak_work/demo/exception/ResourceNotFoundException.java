package com.sarthak_work.demo.exception;

public class ResourceNotFoundException extends RuntimeException{ 
    public ResourceNotFoundException(String message) {
        super(message);
    }
    
}
