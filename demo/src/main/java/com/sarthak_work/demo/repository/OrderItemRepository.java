package com.sarthak_work.demo.repository;

import org.springframework.data.jpa.repository.JpaRepository;

import com.sarthak_work.demo.model.OrderItem;

public interface OrderItemRepository extends JpaRepository<OrderItem , Long>{
    
}
