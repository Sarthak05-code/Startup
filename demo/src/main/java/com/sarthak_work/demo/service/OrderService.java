package com.sarthak_work.demo.service;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.List;

import org.springframework.stereotype.Service;

import com.sarthak_work.demo.dto.OrderItemRequest;
import com.sarthak_work.demo.dto.OrderRequest;
import com.sarthak_work.demo.exception.InsufficientStockException;
import com.sarthak_work.demo.exception.ResourceNotFoundException;
import com.sarthak_work.demo.model.Order;
import com.sarthak_work.demo.model.OrderItem;
import com.sarthak_work.demo.model.OrderStatus;
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
        order.setStatus(OrderStatus.PENDING);

        for (OrderItemRequest itemRequest : orderRequest.getItems()) {
            Product product = productRepository.findById(itemRequest.getProductId())
                    .orElseThrow(() -> new ResourceNotFoundException(
                            "Can't find the product: " + itemRequest.getProductId()));

            if (product.getStockQuantity() < itemRequest.getQuantity()) {
                throw new InsufficientStockException("Not enough stock for: " + product.getName());
            }

            BigDecimal priceOfItem = product.getPrice().multiply(BigDecimal.valueOf(itemRequest.getQuantity()));
            totalPrice = totalPrice.add(priceOfItem);

            product.setStockQuantity(product.getStockQuantity() - itemRequest.getQuantity());
            productRepository.save(product);

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

    public Order getOrderById(Long id) {
        return orderRepository.findById(id)
                .orElseThrow(() -> new ResourceNotFoundException("Order not found: " + id));
    }
}