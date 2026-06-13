package com.sarthak_work.demo.service;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.List;


import org.springframework.stereotype.Service;

import org.springframework.web.bind.annotation.PathVariable;

import com.sarthak_work.demo.dto.OrderItemRequest;
import com.sarthak_work.demo.dto.OrderRequest;
import com.sarthak_work.demo.model.Order;
import com.sarthak_work.demo.model.OrderItem;
import com.sarthak_work.demo.model.Product;
import com.sarthak_work.demo.repository.OrderRepository;
import com.sarthak_work.demo.repository.ProductRepository;

import jakarta.transaction.Transactional;
import lombok.RequiredArgsConstructor;

@Service
@RequiredArgsConstructor
public class OrderService {

    private final OrderRepository orderRepository;
    private final ProductRepository productRepository;

    @Transactional
    public Order createOrder(OrderRequest orderRequest) {
        List<OrderItem> orderItems = new ArrayList<>();
        BigDecimal totalPrice = BigDecimal.ZERO;
        Order order = new Order();
        order.setCustomerName(orderRequest.getCustomerName());
        order.setCustomerEmail(orderRequest.getCustomerEmail());
        order.setStatus("Confirmed");

        for(OrderItemRequest itemRequest : orderRequest.getItems()) {
                Product product = productRepository.findById(itemRequest.getProductId()).orElseThrow(() -> new RuntimeException("Can't fine the product" + itemRequest.getProductId()));
                
                // check the product stock
                if(product.getStockQuantity() < itemRequest.getQuantity()) {
                    throw new RuntimeException("Not enough stock!");
                }

                //Calculate total price
                BigDecimal priceOfItem = product.getPrice().multiply(BigDecimal.valueOf(itemRequest.getQuantity()));
                totalPrice = totalPrice.add(priceOfItem);

                //update stock
                product.setStockQuantity(product.getStockQuantity() - itemRequest.getQuantity())
                ;
                productRepository.save(product);

                //Builder pattern
                OrderItem orderItem = OrderItem.builder()
                        .order(order)
                        .product(product)
                        .quantity(itemRequest.getQuantity())
                        .priceAtPurchase(product.getPrice())
                        .build();

                orderItems.add(orderItem);
        }

        order.setTotalPrice(totalPrice);
        order.setOrderItem(orderItems);
        return orderRepository.save(order);
    }


    
    public List<Order> getAllOrders() {
        return orderRepository.findAll();
    }

    
    public Order getOrderById(@PathVariable Long id) {
        return orderRepository.findById(id).orElseThrow(() -> new RuntimeException("The product id wasn't able to be found."));
    }
    
}
