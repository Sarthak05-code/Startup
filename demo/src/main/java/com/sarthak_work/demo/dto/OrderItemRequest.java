package com.sarthak_work.demo.dto;

import jakarta.validation.constraints.Min;
import jakarta.validation.constraints.NotNull;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;


@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class OrderItemRequest {
    @NotNull(message = "Product id required. ")
    private String productId;

    @NotNull(message = "Quantity required. ")
    @Min(value = 1 , message = "Quantity must be greater than 1.")
    private Integer quantity;
}
