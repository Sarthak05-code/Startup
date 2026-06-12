package com.sarthak_work.demo.model;

import java.math.BigDecimal;
import java.time.LocalDateTime;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.PrePersist;
import jakarta.persistence.Table;
import jakarta.validation.constraints.DecimalMin;
import jakarta.validation.constraints.NotBlank;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Table (name = "orders")
@Entity
@AllArgsConstructor
@NoArgsConstructor
@Builder
public class Order {
    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @NotBlank(message = "Name Required")
    @Column(name = "customer_name" , nullable = false)
    private String customerName;

    @NotBlank(message = "Email Required")
    @Column(name = "customer_email" , nullable = false)
    private String customerEmail;

    @Column(nullable = false)
    private String status;

    @DecimalMin(value = "0.0" , inclusive = true , message = "Price can't be less than zero")
    @Column(name = "total_price" , nullable = false)
    private BigDecimal totalPrice;

    @Column(name = "created_at")
    private LocalDateTime createdAt;

    @PrePersist
    public void prePersist() {
        this.createdAt = LocalDateTime.now();
        if(this.status == null) {
            this.status = "PENDING";
        }
    }

}
