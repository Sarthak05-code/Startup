package com.sarthak_work.demo.dto;

import java.util.List;

import jakarta.validation.Valid;
import jakarta.validation.constraints.Email;
import jakarta.validation.constraints.NotBlank;
import jakarta.validation.constraints.NotEmpty;
import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class OrderRequest {
    @NotBlank(message = "Customer name must be registered.")
    private String customerName;

    @NotBlank(message = "Customer email must be registered.")
    @Email(message = "Invalid email format.")
    private String customerEmail;

    @Valid
    @NotEmpty(message = "Order must contain at least one item.")
    private List<OrderItemRequest> items;
}
