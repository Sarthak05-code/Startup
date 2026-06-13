package com.sarthak_work.demo.repository;

import org.springframework.data.jpa.repository.JpaRepository;

import com.sarthak_work.demo.model.Product;

public interface ProductRepository extends JpaRepository<Product , Long> {
    
}
