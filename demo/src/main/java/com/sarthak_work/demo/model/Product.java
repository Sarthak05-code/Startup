package com.sarthak_work.demo.model;

import java.math.BigDecimal;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.Table;
import jakarta.validation.constraints.DecimalMin;
import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotNull;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data // generates getters, setters, toString, equals, and hashCode
@AllArgsConstructor // generates a constructor with all fields as arguments
@NoArgsConstructor // generates a no-argument constructor
@Entity // marks this class as a JPA entity
@Table(name = "products") // sets the table name; if omitted, defaults to the class name
@Builder // enables builder-pattern style object creation
public class Product {

    @Id // marks this field as the primary key
    @GeneratedValue(strategy = GenerationType.IDENTITY) // auto-incremented by the database
    private Long id;

    @NotBlank(message = "Product name is needed")
    @Column(nullable = false)
    private String name;

    private String description;

    private String category;

    @NotNull(message = "Price is required. ")
    @DecimalMin(value = "0.0", inclusive = false, message = "Price can't be below zero")
    @Column(nullable = false)
    private BigDecimal price;

    @NotNull(message = "Quantity Required ")
    @Min(value = 0, message = "Quantity can't be less than zero. ")
    @Column(name = "stock_quantity", nullable = false)
    private Integer stockQuantity;

    // TODO : relations
}