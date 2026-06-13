package com.sarthak_work.demo.repository;

import org.springframework.data.jpa.repository.JpaRepository;

import com.sarthak_work.demo.model.Order;

public interface OrderRepository extends JpaRepository<Order , Long>{
    
}
